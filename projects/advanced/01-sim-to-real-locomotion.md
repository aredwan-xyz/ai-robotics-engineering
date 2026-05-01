# Project: Sim-to-Real Quadruped Locomotion with RL

**Difficulty:** 🟥 Advanced  
**Skills:** Isaac Lab, PPO, RMA, ROS 2, motor control, system identification  
**Estimated Time:** 2–4 months  
**Hardware Cost:** $800–$2000  
**Compute:** RTX 3080+ (24h training), or AWS p3.2xlarge

---

## What You'll Build

A quadruped robot that learns to walk in simulation via deep reinforcement learning, then successfully transfers that policy to real hardware — achieving stable locomotion without any real-world fine-tuning.

The approach follows the **RMA (Rapid Motor Adaptation)** framework from Kumar et al. (Berkeley/CMU), adapted for a low-cost platform.

## What You'll Learn

- GPU-parallel robot simulation in Isaac Lab
- PPO with asymmetric actor-critic (privileged learning)
- Domain randomization for sim-to-real
- RMA: teacher-student policy distillation
- Real-time inference on embedded hardware
- CAN bus motor control at 500Hz
- System identification for physics parameter estimation

## Hardware

**Option A: Unitree Go1 / Go2 (recommended)**
- Go1 Edu: $2700 (used ~$1000–1500)
- Go2: $1600, open SDK, actively maintained
- Best starting point: well-documented, active community

**Option B: Build from scratch (advanced)**
- 12× Unitree M0601 actuators (~$150 each = $1800)
- 3D-printed/machined frame (~$200)
- Controller PC: Raspberry Pi CM4 or Jetson Orin Nano (~$200)
- Total: ~$2200+

**Option C: Mini quadruped (budget)**
- Unitree Go2 Air: $600 (limited SDK)
- Petoi Bittle X: $300 (hobbyist, limited torque)
- SpotMicro: $400 printed (very limited)

## Software Setup

```bash
# 1. Install Isaac Lab
git clone https://github.com/isaac-sim/IsaacLab.git
cd IsaacLab
./isaaclab.sh --install

# 2. Clone our training code
git clone https://github.com/yourusername/quad-locomotion-rma
cd quad-locomotion-rma

# 3. Install dependencies
pip install -r requirements.txt
```

## Step 1: Define the Robot in MJCF/USD

```python
# robot_config.py — Go2 robot configuration for Isaac Lab
import isaaclab.sim as sim_utils
from isaaclab.assets import ArticulationCfg
from isaaclab.actuators import DCMotorCfg

GO2_CFG = ArticulationCfg(
    spawn=sim_utils.UsdFileCfg(
        usd_path="robots/go2/go2.usd",
        activate_contact_sensors=True,
    ),
    init_state=ArticulationCfg.InitialStateCfg(
        pos=(0.0, 0.0, 0.42),  # 42cm off ground
        joint_pos={
            "FL_hip_joint": 0.0,   "FR_hip_joint": 0.0,
            "RL_hip_joint": 0.0,   "RR_hip_joint": 0.0,
            "FL_thigh_joint": 0.8, "FR_thigh_joint": 0.8,
            "RL_thigh_joint": 1.0, "RR_thigh_joint": 1.0,
            "FL_calf_joint": -1.5, "FR_calf_joint": -1.5,
            "RL_calf_joint": -1.5, "RR_calf_joint": -1.5,
        },
    ),
    actuators={
        "base_legs": DCMotorCfg(
            joint_names_expr=[".*_hip_joint", ".*_thigh_joint", ".*_calf_joint"],
            effort_limit=23.7,   # Nm — Go2 motor specs
            velocity_limit=30.0, # rad/s
            stiffness={".*": 25.0},
            damping={".*": 0.5},
        ),
    },
)
```

## Step 2: Define the RL Environment

```python
# locomotion_env.py
import torch
import isaaclab.envs as envs
from isaaclab.envs import ManagerBasedRLEnvCfg
from isaaclab.managers import (
    ObservationGroupCfg, RewardTermCfg, TerminationTermCfg
)
import isaaclab_tasks.locomotion.velocity.mdp as mdp

@configclass
class QuadLocomotionEnvCfg(ManagerBasedRLEnvCfg):
    # 4096 parallel envs on GPU
    scene: QuadSceneCfg = QuadSceneCfg(num_envs=4096, env_spacing=2.5)
    
    # Observations (what the policy sees)
    observations: ObservationsCfg = ObservationsCfg()
    
    # Rewards (what we optimize)
    rewards: RewardsCfg = RewardsCfg()
    
    # Terminations (when to reset)
    terminations: TerminationsCfg = TerminationsCfg()
    
    # Curriculum (difficulty schedule)
    curriculum: CurriculumCfg = CurriculumCfg()

@configclass
class ObservationsCfg:
    """Observation space definition."""
    
    @configclass
    class PolicyObs(ObservationGroupCfg):
        """Student policy observations — what's available on real hardware."""
        
        # Base velocity estimate (from IMU integration)
        base_lin_vel = ObsTerm(func=mdp.base_lin_vel)
        # Base angular velocity (IMU)
        base_ang_vel = ObsTerm(func=mdp.base_ang_vel)
        # Projected gravity (IMU — tells us orientation)
        projected_gravity = ObsTerm(func=mdp.projected_gravity)
        # Velocity command
        velocity_commands = ObsTerm(func=mdp.generated_commands, params={"command_name": "base_velocity"})
        # Joint positions (encoders)
        joint_pos = ObsTerm(func=mdp.joint_pos_rel)
        # Joint velocities (encoders)
        joint_vel = ObsTerm(func=mdp.joint_vel_rel)
        # Previous actions
        last_action = ObsTerm(func=mdp.last_action)
        
        # Add Gaussian noise to simulate real sensor noise
        def __post_init__(self):
            self.enable_corruption = True
            self.base_lin_vel.noise = GaussianNoiseCfg(std=0.1)
            self.joint_pos.noise = GaussianNoiseCfg(std=0.01)
    
    @configclass 
    class CriticObs(ObservationGroupCfg):
        """Critic observations — privileged info only available in sim."""
        
        # Everything the policy sees
        policy_obs = ObsTerm(func=...)  # include policy obs
        
        # PRIVILEGED (unavailable on real hardware):
        # Ground truth base velocity
        base_lin_vel_true = ObsTerm(func=mdp.base_lin_vel)
        # Foot contact forces
        contact_forces = ObsTerm(func=mdp.contact_forces, params={"threshold": 1.0})
        # Friction coefficients (what ground the robot is on)
        friction_coeff = ObsTerm(func=get_friction_coeff)
        # Body mass perturbation
        mass_perturbation = ObsTerm(func=get_mass_perturbation)
        # Ground height beneath each foot
        heights_scan = ObsTerm(func=mdp.height_scan, params={"sensor_cfg": SceneEntityCfg("height_scanner")})
    
    policy: PolicyObs = PolicyObs()
    critic: CriticObs = CriticObs()

@configclass
class RewardsCfg:
    """Reward function design."""
    
    # Primary task: track commanded velocity
    track_lin_vel_xy = RewardTermCfg(
        func=mdp.track_lin_vel_xy_exp,
        weight=2.0,
        params={"command_name": "base_velocity", "std": 0.25}
    )
    track_ang_vel_z = RewardTermCfg(
        func=mdp.track_ang_vel_z_exp,
        weight=0.5,
        params={"command_name": "base_velocity", "std": 0.25}
    )
    
    # Regularization (penalize bad behaviors)
    lin_vel_z = RewardTermCfg(func=mdp.lin_vel_z_l2, weight=-2.0)
    ang_vel_xy = RewardTermCfg(func=mdp.ang_vel_xy_l2, weight=-0.05)
    action_rate = RewardTermCfg(func=mdp.action_rate_l2, weight=-0.01)
    joint_acc = RewardTermCfg(func=mdp.joint_acc_l2, weight=-2.5e-7)
    
    # Style rewards (encourage natural gait)
    feet_air_time = RewardTermCfg(
        func=mdp.feet_air_time,
        weight=0.5,
        params={"command_name": "base_velocity", "threshold": 0.5}
    )
    
    # Safety (prevent falls)
    flat_orientation = RewardTermCfg(func=mdp.flat_orientation_l2, weight=-5.0)
    dof_pos_limits = RewardTermCfg(func=mdp.joint_pos_limits, weight=-1.0)
    dof_vel_limits = RewardTermCfg(func=mdp.joint_vel_limits, weight=-1.0)
    dof_torque_limits = RewardTermCfg(func=mdp.applied_torque_limits, weight=-1.0)
```

## Step 3: Domain Randomization

```python
# domain_rand.py — randomize physics at episode reset
@configclass
class RandomizationCfg:
    """Domain randomization configuration."""
    
    # Randomize on every reset
    physics_material = RandTerm(
        func=mdp.randomize_rigid_body_material,
        mode="reset",
        params={
            "asset_cfg": SceneEntityCfg("robot", body_names=".*"),
            "static_friction_range": (0.3, 1.5),
            "dynamic_friction_range": (0.3, 1.5),
            "restitution_range": (0.0, 0.1),
        },
    )
    
    robot_mass = RandTerm(
        func=mdp.randomize_rigid_body_mass,
        mode="reset",
        params={
            "asset_cfg": SceneEntityCfg("robot", body_names="base"),
            "mass_distribution_params": (-1.5, 2.5),  # ±Kg
            "operation": "add",
        },
    )
    
    joint_stiffness_and_damping = RandTerm(
        func=mdp.randomize_actuator_gains,
        mode="reset",
        params={
            "asset_cfg": SceneEntityCfg("robot", joint_names=".*"),
            "stiffness_distribution_params": (0.75, 1.5),  # scale factor
            "damping_distribution_params": (0.75, 1.5),
            "operation": "scale",
        },
    )
    
    # Push robot during episode (simulates disturbances)
    push_robot = RandTerm(
        func=mdp.push_by_setting_velocity,
        mode="interval",
        interval_range_s=(5.0, 10.0),  # push every 5-10 seconds
        params={"velocity_range": {"x": (-0.5, 0.5), "y": (-0.5, 0.5)}},
    )
```

## Step 4: Train Teacher Policy

```python
# train_teacher.py
from isaaclab.envs import ManagerBasedRLEnv
from rsl_rl.runners import OnPolicyRunner

env = ManagerBasedRLEnv(cfg=QuadLocomotionEnvCfg())

# PPO training configuration
runner_cfg = OnPolicyRunnerCfg(
    num_steps_per_env=24,     # rollout steps
    max_iterations=3000,       # training iterations
    
    policy=ActorCriticCfg(
        actor_hidden_dims=[512, 256, 128],
        critic_hidden_dims=[512, 256, 128],
        activation="elu",
    ),
    
    algorithm=PPOCfg(
        value_loss_coef=1.0,
        use_clipping=True,
        clip_param=0.2,
        entropy_coef=0.01,
        num_learning_epochs=5,
        num_mini_batches=4,
        learning_rate=1e-3,
        gamma=0.99,
        lam=0.95,
    ),
)

runner = OnPolicyRunner(env, runner_cfg, log_dir="logs/teacher/")
runner.learn(num_learning_iterations=3000)
# ~2-4 hours on RTX 3090
# Final reward: ~8-12 (depends on terrain curriculum)
```

## Step 5: Distill to Student Policy (RMA)

```python
# train_student.py — distill teacher's implicit estimation to student
# Student only sees obs available on real hardware, but must match teacher

class StudentNet(nn.Module):
    """
    Uses a GRU to encode observation history → estimate privileged info latent.
    History of 50 steps = 50 × 0.02s = 1 second of history.
    """
    def __init__(self, obs_dim=45, action_dim=12, history_len=50):
        super().__init__()
        self.history_len = history_len
        
        # Encode observation history
        self.adaptation_module = nn.GRU(
            input_size=obs_dim,
            hidden_size=256,
            num_layers=2,
            batch_first=True
        )
        
        # Same MLP as teacher actor
        self.actor = nn.Sequential(
            nn.Linear(256 + obs_dim, 512),
            nn.ELU(),
            nn.Linear(512, 256),
            nn.ELU(),
            nn.Linear(256, 128),
            nn.ELU(),
            nn.Linear(128, action_dim),
        )
    
    def forward(self, obs_history, current_obs):
        _, latent = self.adaptation_module(obs_history)
        latent = latent[-1]  # last layer hidden state
        combined = torch.cat([current_obs, latent], dim=-1)
        return self.actor(combined)

def train_student(teacher_policy, student_net, env):
    """
    Phase 2: DAgger-style distillation.
    Run teacher in environment, collect (obs_history, teacher_action) pairs.
    Train student to match teacher's actions.
    """
    optimizer = torch.optim.Adam(student_net.parameters(), lr=1e-4)
    
    for iteration in range(1000):
        # Collect data with teacher
        obs_histories, teacher_actions = collect_teacher_rollouts(
            env, teacher_policy, n_steps=10000)
        
        # Supervised learning: student matches teacher
        pred_actions = student_net(obs_histories, obs_histories[:, -1, :])
        loss = F.mse_loss(pred_actions, teacher_actions)
        
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        
        if iteration % 100 == 0:
            print(f"Iter {iteration}: Loss = {loss.item():.4f}")
    
    # Evaluate on real robot after this!
```

## Step 6: Deploy to Real Hardware

```python
# robot_inference.py — runs at 500Hz on robot's onboard computer
import torch
import numpy as np
from unitree_sdk2py.core.channel import ChannelSubscriber, ChannelPublisher
from unitree_sdk2py.idl.unitree_go.msg.dds_ import LowState_, LowCmd_

class RobotController:
    def __init__(self, model_path: str):
        # Load student policy
        self.policy = StudentNet(obs_dim=45, action_dim=12, history_len=50)
        self.policy.load_state_dict(torch.load(model_path, map_location='cpu'))
        self.policy.eval()
        
        # Observation history buffer
        self.obs_buffer = torch.zeros(1, 50, 45)
        
        # ROS 2 or SDK interface
        self.state_sub = ChannelSubscriber("rt/lowstate", LowState_)
        self.cmd_pub = ChannelPublisher("rt/lowcmd", LowCmd_)
        
        # Safety limits
        self.joint_pos_limits = torch.tensor([
            [-0.8, 0.8],   # hip abduction
            [-1.2, 3.4],   # hip flexion
            [-2.8, -0.8],  # knee
        ] * 4)  # 4 legs × 3 joints
        
    def get_observation(self, state: LowState_) -> torch.Tensor:
        """Convert robot state to policy observation."""
        # IMU data
        ang_vel = torch.tensor([
            state.imu_state.gyroscope[0],
            state.imu_state.gyroscope[1],
            state.imu_state.gyroscope[2],
        ])
        
        # Gravity projection from quaternion
        quat = torch.tensor(state.imu_state.quaternion)
        gravity_body = quat_rotate_inverse(quat, torch.tensor([0., 0., -1.]))
        
        # Joint states (12 joints: FL/FR/RL/RR × hip/thigh/calf)
        joint_pos = torch.tensor([state.motor_state[i].q for i in range(12)])
        joint_vel = torch.tensor([state.motor_state[i].dq for i in range(12)])
        
        # Target velocity command (from joystick or planner)
        vel_cmd = self.get_velocity_command()  # [vx, vy, wz]
        
        # Previous action
        prev_action = self.last_action
        
        obs = torch.cat([
            ang_vel,        # 3
            gravity_body,   # 3
            vel_cmd,        # 3
            joint_pos,      # 12
            joint_vel,      # 12
            prev_action,    # 12
        ])  # total: 45
        
        return obs
    
    @torch.no_grad()
    def control_step(self, state: LowState_) -> LowCmd_:
        """Single control step — called at 500Hz."""
        obs = self.get_observation(state)
        
        # Update history buffer (FIFO)
        self.obs_buffer = torch.roll(self.obs_buffer, -1, dims=1)
        self.obs_buffer[0, -1, :] = obs
        
        # Policy inference
        action = self.policy(self.obs_buffer, obs.unsqueeze(0))
        action = action.squeeze(0)
        
        # Safety: clip to joint limits
        target_pos = self.default_joint_pos + action * self.action_scale
        target_pos = torch.clamp(target_pos, 
                                 self.joint_pos_limits[:, 0],
                                 self.joint_pos_limits[:, 1])
        
        self.last_action = action
        
        # Build motor command
        cmd = LowCmd_()
        for i in range(12):
            cmd.motor_cmd[i].q = float(target_pos[i])
            cmd.motor_cmd[i].dq = 0.0
            cmd.motor_cmd[i].kp = 25.0   # stiffness
            cmd.motor_cmd[i].kd = 0.5    # damping
            cmd.motor_cmd[i].tau = 0.0   # feedforward torque
        
        return cmd
    
    def run(self):
        """Main control loop at 500Hz."""
        rate = 500  # Hz
        dt = 1.0 / rate
        
        self.state_sub.Init()
        self.cmd_pub.Init()
        
        print("Starting control loop at 500Hz")
        while True:
            t_start = time.monotonic()
            
            state = self.state_sub.Read()
            if state is not None:
                cmd = self.control_step(state)
                self.cmd_pub.Write(cmd)
            
            # Precisely control loop rate
            elapsed = time.monotonic() - t_start
            sleep_time = dt - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
```

## Evaluation Protocol

```python
# evaluation.py — systematic evaluation on real hardware

def evaluate_policy(robot, policy, num_episodes=20):
    results = {
        'success_rate': 0,
        'avg_velocity_tracking': [],
        'fall_rate': 0,
        'energy_consumption': [],
    }
    
    for ep in range(num_episodes):
        # Random goal velocity
        vx = np.random.uniform(0, 1.0)   # 0–1 m/s forward
        vy = np.random.uniform(-0.3, 0.3) # lateral
        wz = np.random.uniform(-0.5, 0.5) # turning
        
        robot.reset()
        policy.set_command(vx, vy, wz)
        
        ep_data = {'tracking': [], 'power': [], 'fell': False}
        
        for step in range(500):  # 10 seconds at 50Hz eval
            state = robot.get_state()
            
            if robot.has_fallen():
                ep_data['fell'] = True
                break
            
            actual_vel = state['base_velocity']
            tracking = 1.0 - np.linalg.norm(actual_vel[:3] - [vx, vy, wz]) / max(np.linalg.norm([vx, vy, wz]), 0.1)
            ep_data['tracking'].append(tracking)
            ep_data['power'].append(state['power_draw'])
        
        results['avg_velocity_tracking'].append(np.mean(ep_data['tracking']))
        results['energy_consumption'].append(np.mean(ep_data['power']))
        if ep_data['fell']:
            results['fall_rate'] += 1
    
    results['fall_rate'] /= num_episodes
    print(f"Success rate: {1-results['fall_rate']:.1%}")
    print(f"Velocity tracking: {np.mean(results['avg_velocity_tracking']):.2f}")
    print(f"Avg power: {np.mean(results['energy_consumption']):.1f}W")
    return results
```

## Expected Results

```
After Stage 1 (teacher training, 3000 iterations):
  Flat ground: 95%+ success rate
  Velocity tracking: ~0.85 (1.0 = perfect)
  
After Stage 2 (student distillation):
  Flat ground: 90%+ success rate (slight degradation from teacher)
  Velocity tracking: ~0.75
  
On real hardware (no fine-tuning):
  Flat ground: 70–85% expected
  Uneven terrain: 50–70%
  After 1-2 hours of real-world data: 85–95%
  
Gap sources:
  Friction: biggest factor — randomize widely in training
  Motor delays: add 1–2 step action delay in sim
  Unmodeled flex: add noise proportional to velocity
```

## Debugging Tips

1. **Policy never walks in sim** → Check reward weights, start with flat ground only
2. **Falls immediately on real hardware** → Motor delay compensation, increase kd damping
3. **Jittery motion** → Increase action_rate penalty, reduce action scale
4. **Won't track velocity commands** → Increase track_lin_vel reward weight
5. **Weird gait** → Add gait regularization rewards, increase feet_air_time weight
6. **Overheating motors** → Add torque penalty, reduce stiffness kp

## Further Reading

- [RMA: Rapid Motor Adaptation for Legged Robots](https://arxiv.org/abs/2107.04034) — The core algorithm
- [Learning to Walk in Minutes](https://arxiv.org/abs/2109.11978) — Fast training approach
- [Isaac Lab Documentation](https://isaac-sim.github.io/IsaacLab/) — Framework reference
- [Unitree Go2 SDK](https://github.com/unitreerobotics/unitree_sdk2) — Robot interface
- [RSL-RL](https://github.com/leggedrobotics/rsl_rl) — RL library used by ETH RSL
