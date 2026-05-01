# Physical AI Engineer Roadmap

> From TinyML on a microcontroller to deploying foundation models on real robots.
> Estimated time: 12–24 months. Requires programming and some ML background.

## What is Physical AI?

Physical AI is the discipline of building AI systems that act in the physical world — robots, autonomous vehicles, smart manufacturing systems, and any AI that must perceive, reason, and act with real-world consequences.

It is distinct from "software AI" in that:
- **Mistakes have physical consequences** (crashes, injuries, damaged objects)
- **Real-time constraints are hard** (you can't wait 10 seconds for GPT to think)
- **Sensor noise and calibration errors are everywhere** (sim ≠ real)
- **The state is continuous and partially observable** (not discrete tokens)
- **Data is expensive to collect** (can't scrape the internet for robot demos)

---

## Prerequisites

**Must have:**
- Python proficiency (numpy, PyTorch/JAX basics)
- Basic ML: linear regression → neural networks → CNNs
- Calculus and linear algebra (same as ML)

**Helpful:**
- ROS 2 basics (helps but not required at start)
- Control theory basics
- Access to a robot (even a cheap servo arm) greatly accelerates learning

---

## Stage 1: TinyML & Edge AI (Months 1–3)

This stage teaches you to run AI at the edge — on constrained hardware.

### Why start here?
- Fast feedback loop: deploy in minutes, not days
- Builds hardware intuition before tackling full robots
- Directly applicable to sensor nodes, wearables, embedded systems

### The Edge AI Workflow

```
Collect data → Label → Train → Quantize → Convert → Deploy → Measure

1. Collect: IMU data, audio, images from real sensor
2. Label: classes, anomalies, events
3. Train: on PC/cloud, standard PyTorch/TF
4. Quantize: INT8 or INT4 to reduce size/latency
5. Convert: to TFLite, ONNX, TVM-compiled binary
6. Deploy: flash to MCU/SBC
7. Measure: latency, accuracy, power draw on real hardware
```

### Quantization Deep Dive

```python
import torch
import torch.nn as nn
from torch.quantization import quantize_dynamic

# Post-Training Dynamic Quantization (easiest)
model_fp32 = MyModel()
model_fp32.load_state_dict(torch.load('model.pth'))
model_fp32.eval()

model_int8 = quantize_dynamic(
    model_fp32,
    {nn.Linear, nn.LSTM},
    dtype=torch.qint8
)
# Typical result: 2-4x smaller, 1.5-2x faster on CPU, ~1% accuracy drop

# Post-Training Static Quantization (better for CNNs)
model.qconfig = torch.quantization.get_default_qconfig('fbgemm')
model_prepared = torch.quantization.prepare(model)
# Run calibration data through model_prepared
model_quantized = torch.quantization.convert(model_prepared)

# Quantization-Aware Training (best accuracy)
model.qconfig = torch.quantization.get_default_qat_qconfig('fbgemm')
model_qat = torch.quantization.prepare_qat(model)
# Train normally — simulates quantization during forward pass
model_quantized = torch.quantization.convert(model_qat.eval())
```

### TFLite Micro on Arduino

```cpp
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "model_data.h"  // your converted model as a C array

const int kTensorArenaSize = 10 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

tflite::AllOpsResolver resolver;
const tflite::Model* model = tflite::GetModel(g_model);
tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize);
interpreter.AllocateTensors();

TfLiteTensor* input = interpreter.input(0);
TfLiteTensor* output = interpreter.output(0);

// Fill input
input->data.f[0] = sensor_reading;

// Run inference
interpreter.Invoke();

// Read output
float result = output->data.f[0];
```

### Projects for This Stage

1. **Keyword spotting** — "yes/no" detection on Arduino Nano 33 BLE Sense
2. **Gesture recognition** — Classify 5 gestures from IMU data on ESP32
3. **Anomaly detection** — Detect bearing faults from vibration on STM32
4. **Visual wake words** — Person/no-person on OV2640 camera + ESP32-S3

### Tools

- [Edge Impulse](https://edgeimpulse.com/) — End-to-end TinyML platform. Use this first.
- [TFLite Micro](https://www.tensorflow.org/lite/microcontrollers)
- [ExecuTorch](https://pytorch.org/executorch/) — Meta's on-device PyTorch
- [Netron](https://netron.app/) — Visualize your neural network graph

---

## Stage 2: Reinforcement Learning Foundations (Months 2–5)

### Core Concepts

```
Agent → Action → Environment → State + Reward → Agent (repeat)

Goal: maximize cumulative reward E[Σ γ^t r_t]

Key concepts:
  State (s):      What the agent observes
  Action (a):     What the agent does
  Reward (r):     Scalar feedback signal
  Policy (π):     s → a (what to do in each state)
  Value (V):      Expected future reward from state
  Q-value:        Expected future reward from (state, action)
  Advantage (A):  Q(s,a) - V(s) — how much better is this action?
```

### Implement from Scratch (do this)

```python
import gymnasium as gym
import numpy as np

# Policy Gradient (REINFORCE) — simplest deep RL algorithm
class PolicyNet(nn.Module):
    def __init__(self, obs_dim, act_dim):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(obs_dim, 64), nn.Tanh(),
            nn.Linear(64, 64), nn.Tanh(),
            nn.Linear(64, act_dim), nn.Softmax(dim=-1)
        )
    def forward(self, x):
        return self.net(x)

env = gym.make('CartPole-v1')
policy = PolicyNet(env.observation_space.shape[0], env.action_space.n)
optimizer = torch.optim.Adam(policy.parameters(), lr=1e-3)

for episode in range(1000):
    obs, _ = env.reset()
    log_probs, rewards = [], []
    
    while True:
        probs = policy(torch.FloatTensor(obs))
        dist = torch.distributions.Categorical(probs)
        action = dist.sample()
        log_probs.append(dist.log_prob(action))
        
        obs, reward, done, _, _ = env.step(action.item())
        rewards.append(reward)
        if done: break
    
    # Compute returns
    G = 0
    returns = []
    for r in reversed(rewards):
        G = r + 0.99 * G
        returns.insert(0, G)
    returns = torch.tensor(returns)
    returns = (returns - returns.mean()) / (returns.std() + 1e-8)
    
    # Update policy
    loss = -torch.stack(log_probs) @ returns
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
```

### Key Algorithms for Robotics

**PPO (Proximal Policy Optimization)** — Standard for locomotion
```python
# Core PPO update (simplified)
# ratio = π_new(a|s) / π_old(a|s)
ratio = (new_log_probs - old_log_probs).exp()
surr1 = ratio * advantages
surr2 = torch.clamp(ratio, 1 - epsilon, 1 + epsilon) * advantages
policy_loss = -torch.min(surr1, surr2).mean()
# The clamp is the "proximal" part — limits how much policy can change
```

**SAC (Soft Actor-Critic)** — Standard for manipulation
```
Key idea: maximize reward AND entropy (be as random as possible while succeeding)
α * H(π) — entropy regularization
→ Exploration is built-in, no need for ε-greedy
→ More stable than deterministic policies
→ Off-policy: sample efficient
```

**HER (Hindsight Experience Replay)** — For sparse rewards
```
Problem: robot tried to put cube at (1,1) but put it at (2,2) → reward = 0
Insight: pretend the goal WAS (2,2) → suddenly reward = 1
→ Learn from failed episodes by relabeling the goal
→ Critical for manipulation with sparse rewards
```

### Resources

- [Spinning Up in Deep RL (OpenAI)](https://spinningup.openai.com/) — Best practical intro
- [Sutton & Barto RL Book](http://incompleteideas.net/book/the-book-2nd.html) — Free PDF, theory bible
- [David Silver RL Course](https://www.deepmind.com/learning-resources/introduction-to-reinforcement-learning-with-david-silver) — DeepMind course
- [Stable-Baselines3](https://stable-baselines3.readthedocs.io/) — Production RL library

---

## Stage 3: Simulation for Robotics (Months 3–6)

**Why simulation is non-negotiable:**
- Real-robot experiments: 1 episode/minute → 1000 hours for 60k episodes
- Simulation: 50,000 environments in parallel → 60k episodes in minutes
- You CANNOT do RL on real hardware without sim (at current scale)

### MuJoCo Fundamentals

```python
import mujoco
import numpy as np

# Load model
model = mujoco.MjModel.from_xml_path('robot.xml')
data = mujoco.MjData(model)

# Simulation step
data.ctrl[0] = 1.0  # set actuator 0 to 1.0 N
mujoco.mj_step(model, data)

# Read state
qpos = data.qpos.copy()  # joint positions
qvel = data.qvel.copy()  # joint velocities
xpos = data.xpos.copy()  # body positions in world frame

# MJCF (MuJoCo XML) robot description
xml = """
<mujoco>
  <worldbody>
    <body name="cart">
      <joint type="slide" axis="1 0 0"/>
      <geom type="box" size="0.1 0.1 0.05"/>
      <body name="pole" pos="0 0 0.1">
        <joint type="hinge" axis="0 1 0"/>
        <geom type="capsule" size="0.02" fromto="0 0 0 0 0 0.3"/>
      </body>
    </body>
  </worldbody>
  <actuator>
    <motor joint="slide" gear="100"/>
  </actuator>
</mujoco>
"""
```

### Isaac Lab (GPU-Parallel Training)

```python
# Isaac Lab enables training 4096 robots simultaneously on a single GPU
# ~1000x faster than CPU simulation

import isaaclab.envs
from isaaclab_tasks.classic.cartpole import CartpoleEnv

env = CartpoleEnv(num_envs=4096, device='cuda')
obs, _ = env.reset()

for _ in range(10000):
    actions = policy(obs)              # 4096 policies in parallel
    obs, reward, done, _, _ = env.step(actions)
    
    if done.any():
        obs[done] = env.reset_idx(done.nonzero())

# Training stats: ~10M steps/second on RTX 4090
```

### Gymnasium Custom Environment

```python
import gymnasium as gym
from gymnasium import spaces

class RobotArmEnv(gym.Env):
    def __init__(self):
        self.observation_space = spaces.Box(
            low=-np.inf, high=np.inf, shape=(14,), dtype=np.float32)
            # joint positions (7) + joint velocities (7)
        self.action_space = spaces.Box(
            low=-1.0, high=1.0, shape=(7,), dtype=np.float32)
            # normalized joint torques
    
    def reset(self, seed=None):
        self.robot.reset_to_random()
        self.goal = self.sample_goal()
        return self._get_obs(), {}
    
    def step(self, action):
        self.robot.apply_torques(action * self.torque_limit)
        self.sim.step()
        
        obs = self._get_obs()
        reward = self._compute_reward()
        terminated = self._check_success()
        truncated = self.step_count > self.max_steps
        
        return obs, reward, terminated, truncated, {}
    
    def _compute_reward(self):
        dist = np.linalg.norm(self.ee_pos - self.goal)
        return -dist  # dense reward: minimize distance
```

---

## Stage 4: Sim-to-Real Transfer (Months 6–9)

**The Reality Gap Problem:**
Training in simulation is cheap. Deploying to real robots is hard because:
- Simulator physics ≠ real physics (contacts, friction, flex)
- Simulated sensors ≠ real sensors (noise, latency, drift)
- Simulated actuators ≠ real actuators (backlash, heat, wear)
- Unmodeled dynamics (cable routing, vibration, air resistance)

### Technique 1: Domain Randomization

```python
# Randomize physics parameters during training
# → Policy learns to be robust to parameter variation
# → Real robot is just one sample from the distribution

class RandomizedEnv:
    def reset(self):
        # Randomize friction
        self.model.geom_friction[:] = np.random.uniform(0.3, 1.5, size=self.model.ngeom)
        
        # Randomize masses (+/- 30%)
        for i in range(self.model.nbody):
            self.model.body_mass[i] *= np.random.uniform(0.7, 1.3)
        
        # Randomize motor gains
        self.model.actuator_gear[:] *= np.random.uniform(0.8, 1.2)
        
        # Add observation noise
        obs = self._get_obs()
        obs += np.random.normal(0, 0.01, obs.shape)
        return obs

# Key insight: if policy works with friction ∈ [0.3, 1.5],
# real robot friction (≈0.7) is within the distribution
```

### Technique 2: Privileged Learning (Teacher-Student)

```python
# Teacher (in sim): has access to privileged info (ground truth physics params)
# Student (on real): only has observations

class TeacherPolicy(nn.Module):
    def __init__(self):
        self.encoder = MLPEncoder(obs_dim + privileged_dim, 256)
        self.head = MLPActor(256, action_dim)
    
    def forward(self, obs, privileged_info):
        # privileged_info = [friction, mass, contact forces, ...]
        x = torch.cat([obs, privileged_info], dim=-1)
        return self.head(self.encoder(x))

class StudentPolicy(nn.Module):
    def __init__(self):
        self.history_encoder = GRU(obs_dim, 256)  # encode observation history
        self.head = MLPActor(256, action_dim)
    
    def forward(self, obs_history):
        # Estimate privileged info from history (implicit adaptation)
        latent = self.history_encoder(obs_history)
        return self.head(latent)

# RMA training: 
# Phase 1: Train teacher with privileged info (easy)
# Phase 2: Distill teacher → student via supervised learning on rollout data
```

### Technique 3: System Identification

```python
# Measure real robot parameters → use in simulation

def identify_motor_friction(robot):
    """Move robot slowly, measure back-EMF to estimate friction"""
    results = []
    for velocity in np.linspace(0, 5, 20):
        robot.set_velocity(velocity)
        time.sleep(0.5)
        torque = robot.get_torque()
        results.append((velocity, torque))
    
    # Fit viscous + Coulomb friction model: τ_friction = b*v + c*sign(v)
    from scipy.optimize import curve_fit
    def friction_model(v, b, c):
        return b * v + c * np.sign(v)
    
    (b, c), _ = curve_fit(friction_model, *zip(*results))
    return b, c  # use these in MJCF/URDF simulation
```

---

## Stage 5: Imitation Learning (Months 8–12)

**Why imitation learning:**
- RL from scratch requires reward engineering (hard, domain-specific)
- Humans can demonstrate tasks naturally
- Cheaper data than supervised ML (just move robot around)
- Current SOTA for dexterous manipulation

### Data Collection

```python
# Teleoperation data collection
# Setup: VR controllers / SpaceMouse / Leader-Follower arm → record

from lerobot.common.datasets.push_dataset_to_hub import push_dataset_to_hub
from lerobot.scripts.control_robot import record

# Record 50 demonstrations for the task
record(
    robot=robot,
    fps=30,
    root="data/",
    repo_id="username/my_task",
    warmup_time_s=2,
    episode_time_s=20,
    num_episodes=50,
    push_to_hub=True
)
```

### ACT (Action Chunking with Transformers)

```python
import torch
import torch.nn as nn
from einops import rearrange

class ACT(nn.Module):
    """
    Learns to predict chunks of actions (not one at a time).
    Key insight: predict 100 future actions at once → smooth, non-jerky motion
    """
    def __init__(self, obs_dim, action_dim, chunk_size=100, hidden_dim=512):
        super().__init__()
        self.chunk_size = chunk_size
        
        # Encode visual observations
        self.image_encoder = ResNet18Backbone()
        self.joint_encoder = nn.Linear(obs_dim, hidden_dim)
        
        # CVAE: learn compact latent representation of action sequences
        self.encoder = nn.TransformerEncoder(...)
        self.decoder = nn.TransformerDecoder(...)
        
        # Predict full chunk of actions
        self.action_head = nn.Linear(hidden_dim, action_dim * chunk_size)
    
    def forward(self, images, joint_states, action_sequence=None):
        img_feat = self.image_encoder(images)
        joint_feat = self.joint_encoder(joint_states)
        
        if action_sequence is not None:  # training
            # Encode action sequence to latent
            z = self.encoder(action_sequence)
            mu, log_std = z.chunk(2, dim=-1)
            z_sample = mu + torch.randn_like(mu) * log_std.exp()
        else:  # inference
            z_sample = torch.zeros(batch, self.hidden_dim)
        
        # Decode: condition on observations + latent
        features = self.decoder(z_sample, torch.cat([img_feat, joint_feat]))
        actions = self.action_head(features)
        return rearrange(actions, 'b (t d) -> b t d', d=action_dim)
```

### Diffusion Policy

```python
# Diffusion Policy: treat action prediction as denoising
# Start with Gaussian noise → denoise into clean action sequence

class DiffusionPolicy(nn.Module):
    def __init__(self, obs_dim, action_dim, horizon=16):
        super().__init__()
        self.horizon = horizon
        self.noise_net = ConditionalUNet1D(
            input_dim=action_dim,
            condition_dim=obs_dim,
            diffusion_step_embed_dim=256
        )
    
    def forward(self, obs, noisy_actions, timestep):
        # Predict noise (or x0 directly)
        return self.noise_net(noisy_actions, timestep, condition=obs)
    
    @torch.no_grad()
    def generate(self, obs, num_inference_steps=100):
        # Start from pure noise
        actions = torch.randn(1, self.horizon, action_dim)
        
        # DDPM sampling
        for t in reversed(range(num_inference_steps)):
            noise_pred = self(obs, actions, t)
            actions = self.ddpm_step(actions, noise_pred, t)
        
        return actions
```

### LeRobot Framework

```bash
# Install
pip install lerobot

# Train ACT on recorded data
python lerobot/scripts/train.py \
  policy=act \
  env=aloha \
  dataset_repo_id=lerobot/aloha_sim_insertion_human \
  training.num_workers=8

# Train Diffusion Policy
python lerobot/scripts/train.py \
  policy=diffusion \
  env=pusht \
  dataset_repo_id=lerobot/pusht

# Evaluate
python lerobot/scripts/eval.py \
  -p outputs/train/act_aloha/checkpoints/last/pretrained_model \
  eval.n_episodes=10
```

---

## Stage 6: Foundation Models for Robotics (Months 12–20)

### Vision-Language-Action Models

```python
# Using OpenVLA (open-source VLA model)
from transformers import AutoModelForVision2Seq, AutoProcessor
from PIL import Image
import torch

processor = AutoProcessor.from_pretrained("openvla/openvla-7b")
vla = AutoModelForVision2Seq.from_pretrained(
    "openvla/openvla-7b",
    attn_implementation="flash_attention_2",
    torch_dtype=torch.bfloat16,
    low_cpu_mem_usage=True,
    trust_remote_code=True
).to("cuda")

# Inference: image + language instruction → robot action
image = Image.open("robot_view.jpg")
prompt = "In: What action should the robot take to pick up the red block?\nOut:"

inputs = processor(prompt, image).to("cuda", dtype=torch.bfloat16)
action = vla.predict_action(**inputs, unnorm_key="bridge_orig", do_sample=False)
# action = 7-DoF end-effector delta [dx, dy, dz, droll, dpitch, dyaw, gripper]
```

### Collecting Data for Fine-tuning

```python
# Fine-tune VLA on your specific robot/task
# Need: ~50-500 demonstrations

# Data format for OpenVLA fine-tuning
episode = {
    "observation.images.wrist_cam": [...],  # list of PIL images
    "observation.state": [...],              # joint states
    "action": [...],                         # robot actions
    "task_description": "pick up the red block and place it in the bin"
}

# Fine-tune with LoRA (efficient)
from peft import LoraConfig, get_peft_model

lora_config = LoraConfig(
    r=32,
    lora_alpha=16,
    target_modules=["q_proj", "v_proj", "k_proj", "o_proj"],
    lora_dropout=0.05,
    bias="none"
)
vla_lora = get_peft_model(vla, lora_config)
# Fine-tune on your data...
```

### Building a Real-Robot Pipeline

```
Full Physical AI Pipeline:

1. Perception:
   Camera → RGB image
   LiDAR → Point cloud (optional)
   Proprioception → Joint states + IMU

2. Foundation Model (VLA or separate modules):
   Image + Language → High-level plan
   Plan + State → Low-level actions

3. Low-level Control:
   Actions → Motor commands
   Force control for contact tasks
   Safety limits + collision avoidance

4. Learning Loop:
   Collect failures → Augment data → Fine-tune → Repeat

Hardware stack for low-cost robot research:
  Camera: USB webcam (~$30) or Intel RealSense D435i ($150)
  Arm: SO-ARM100 ($100 printable) or Feetech STS3215-based ($150)
  Compute: Jetson Orin Nano ($199) or local RTX GPU workstation
  Teleop: SpaceMouse ($100) or leader-follower arm (build one for $100)
```

---

## Resources: Physical AI

**Must-follow researchers:**
- Chelsea Finn (Stanford) — Meta-learning, imitation learning
- Sergey Levine (UC Berkeley RAIL) — Robot learning
- Pieter Abbeel (UC Berkeley, Covariant) — Learning from demos
- Russ Tedrake (MIT) — Drake, underactuated robotics
- Ani Majumdar (Princeton) — Sim-to-real, adaptive control
- Tony Zhao (Stanford, ACT paper) — ACT, ALOHA

**Key Labs:**
- [Berkeley RAIL](https://rail.eecs.berkeley.edu/)
- [Stanford IRIS](https://irislab.stanford.edu/)
- [MIT CSAIL Robotics](https://www.csail.mit.edu/)
- [ETH RSL](https://rsl.ethz.ch/)
- [Physical Intelligence](https://www.physicalintelligence.company/)

**Papers to track:**
- [arxiv cs.RO daily](https://arxiv.org/list/cs.RO/recent)
- [Yannic Kilcher's channel](https://www.youtube.com/@YannicKilcher)

**Datasets to explore:**
- [Open X-Embodiment](https://robotics-transformer-x.github.io/) — 22 robots, 1M episodes
- [BridgeData V2](https://rail-berkeley.github.io/bridgedata/) — Kitchen manipulation
- [DROID](https://droid-dataset.github.io/) — 76k episodes, 564 tasks
