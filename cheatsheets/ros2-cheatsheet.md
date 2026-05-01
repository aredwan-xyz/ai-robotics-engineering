# ROS 2 Command Cheatsheet

## Setup & Workspace

```bash
# Source ROS 2
source /opt/ros/humble/setup.bash
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# Create workspace
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws

# Build
colcon build
colcon build --symlink-install          # symlink Python files (no rebuild needed)
colcon build --packages-select my_pkg   # build specific package
colcon build --parallel-workers 4       # limit parallel builds

# Source workspace
source install/setup.bash
echo "source ~/ros2_ws/install/setup.bash" >> ~/.bashrc

# Clean build
rm -rf build/ install/ log/
```

## Package Management

```bash
# Create package
ros2 pkg create --build-type ament_python my_python_pkg
ros2 pkg create --build-type ament_cmake my_cpp_pkg
ros2 pkg create --build-type ament_python \
  --dependencies rclpy std_msgs geometry_msgs nav_msgs \
  my_robot_pkg

# List packages
ros2 pkg list
ros2 pkg list | grep nav2

# Package info
ros2 pkg executables my_pkg
ros2 pkg prefix my_pkg
```

## Nodes

```bash
# Run a node
ros2 run <package> <executable>
ros2 run turtlesim turtlesim_node

# Launch file
ros2 launch <package> <launch_file>
ros2 launch nav2_bringup navigation_launch.py

# Node info
ros2 node list
ros2 node info /my_node
ros2 node info /turtlesim
```

## Topics

```bash
# List topics
ros2 topic list
ros2 topic list -t   # show message types

# Topic info
ros2 topic info /cmd_vel
ros2 topic info /scan --verbose

# Echo messages
ros2 topic echo /cmd_vel
ros2 topic echo /scan --no-arr    # suppress arrays
ros2 topic echo /image_raw --once # single message

# Publishing from command line
ros2 topic pub /cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.5}, angular: {z: 0.3}}'

ros2 topic pub --once /cmd_vel geometry_msgs/msg/Twist \
  '{linear: {x: 0.0}, angular: {z: 0.0}}'

# Topic statistics
ros2 topic hz /scan          # publishing rate
ros2 topic bw /image_raw     # bandwidth
ros2 topic delay /scan       # message delay (needs timestamp)

# Find topics by type
ros2 topic find sensor_msgs/msg/LaserScan
```

## Services

```bash
# List services
ros2 service list
ros2 service list -t

# Service info
ros2 service type /spawn
ros2 service find std_srvs/srv/Empty

# Call service
ros2 service call /spawn turtlesim/srv/Spawn \
  '{x: 2.0, y: 3.0, theta: 0.0, name: "turtle2"}'

ros2 service call /clear std_srvs/srv/Empty '{}'
ros2 service call /kill turtlesim/srv/Kill '{name: "turtle1"}'
```

## Actions

```bash
# List actions
ros2 action list
ros2 action list -t

# Action info
ros2 action info /navigate_to_pose --ros-args

# Send goal
ros2 action send_goal /navigate_to_pose \
  nav2_msgs/action/NavigateToPose \
  '{pose: {pose: {position: {x: 1.0, y: 0.0}}}}'

# With feedback
ros2 action send_goal --feedback /navigate_to_pose \
  nav2_msgs/action/NavigateToPose \
  '{pose: {pose: {position: {x: 1.0, y: 0.0}}}}'
```

## Parameters

```bash
# List parameters
ros2 param list
ros2 param list /my_node

# Get/set
ros2 param get /my_node update_rate
ros2 param set /my_node update_rate 20.0

# Dump to file
ros2 param dump /my_node
ros2 param dump /my_node --output-dir /tmp/

# Load from file
ros2 param load /my_node params.yaml
```

## TF2 (Transforms)

```bash
# View all frames
ros2 run tf2_tools view_frames
# → Generates frames.pdf showing TF tree

# Echo transform between frames
ros2 run tf2_ros tf2_echo world base_link
ros2 run tf2_ros tf2_echo map odom

# Static transform
ros2 run tf2_ros static_transform_publisher \
  0.1 0 0.2 0 0 0 base_link camera_link
# x y z roll pitch yaw parent child

# TF monitor (debug latency)
ros2 run tf2_ros tf2_monitor
```

## Bag Files

```bash
# Record
ros2 bag record /topic1 /topic2 /topic3
ros2 bag record -o my_bag /scan /odom /cmd_vel
ros2 bag record -a   # record all topics (large!)

# Info
ros2 bag info my_bag.db3

# Play
ros2 bag play my_bag.db3
ros2 bag play my_bag.db3 --rate 0.5   # half speed
ros2 bag play my_bag.db3 --loop       # repeat

# Convert (if needed)
ros2 bag convert -i input.db3 -o output.db3 -s mcap
```

## RViz2

```bash
# Launch RViz2
ros2 run rviz2 rviz2
ros2 run rviz2 rviz2 -d my_config.rviz

# Common displays to add:
# - RobotModel (uses /robot_description + TF)
# - LaserScan (topic: /scan)
# - PointCloud2 (topic: /cloud)
# - Image (topic: /image_raw)
# - Map (topic: /map)
# - Path (topic: /plan)
# - Odometry (topic: /odom)
# - TF (shows all coordinate frames)
# - MarkerArray (for custom visualization)
```

## rqt Tools

```bash
# Main rqt window
rqt

# Specific tools
ros2 run rqt_graph rqt_graph          # node/topic graph
ros2 run rqt_plot rqt_plot            # real-time plotting
ros2 run rqt_console rqt_console      # log viewer
ros2 run rqt_image_view rqt_image_view
ros2 run rqt_tf_tree rqt_tf_tree      # TF tree viewer
```

## Python Node Template

```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import LaserScan
import numpy as np

class MyRobotNode(Node):
    def __init__(self):
        super().__init__('my_robot_node')

        # Declare parameters
        self.declare_parameter('update_rate', 10.0)
        self.declare_parameter('max_speed', 0.5)
        self.rate = self.get_parameter('update_rate').value

        # QoS profile for sensor data
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            durability=DurabilityPolicy.VOLATILE,
            depth=1
        )

        # Publisher
        self.cmd_pub = self.create_publisher(Twist, 'cmd_vel', 10)

        # Subscriber with QoS
        self.scan_sub = self.create_subscription(
            LaserScan, 'scan', self.scan_callback, sensor_qos)

        # Timer
        self.timer = self.create_timer(1.0 / self.rate, self.control_loop)

        self.latest_scan = None
        self.get_logger().info('Node initialized')

    def scan_callback(self, msg: LaserScan):
        self.latest_scan = msg

    def control_loop(self):
        if self.latest_scan is None:
            return

        ranges = np.array(self.latest_scan.ranges)
        ranges = np.where(np.isfinite(ranges), ranges, 10.0)

        # Simple obstacle avoidance
        front_dist = np.min(ranges[len(ranges)//2-20 : len(ranges)//2+20])

        cmd = Twist()
        if front_dist > 0.5:
            cmd.linear.x = 0.3
        else:
            cmd.angular.z = 0.5  # turn in place

        self.cmd_pub.publish(cmd)

def main(args=None):
    rclpy.init(args=args)
    node = MyRobotNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
```

## C++ Node Template

```cpp
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

class MyRobotNode : public rclcpp::Node {
public:
    MyRobotNode() : Node("my_robot_node") {
        // Parameters
        this->declare_parameter("max_speed", 0.5);
        max_speed_ = this->get_parameter("max_speed").as_double();

        // Publisher
        cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

        // Subscriber
        scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "scan", rclcpp::SensorDataQoS(),
            std::bind(&MyRobotNode::scanCallback, this, std::placeholders::_1));

        // Timer (10Hz)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(100),
            std::bind(&MyRobotNode::controlLoop, this));

        RCLCPP_INFO(this->get_logger(), "Node initialized");
    }

private:
    void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        latest_scan_ = msg;
    }

    void controlLoop() {
        if (!latest_scan_) return;

        auto cmd = geometry_msgs::msg::Twist();
        cmd.linear.x = max_speed_ * 0.5;
        cmd_pub_->publish(cmd);
    }

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    sensor_msgs::msg::LaserScan::SharedPtr latest_scan_;
    double max_speed_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MyRobotNode>());
    rclcpp::shutdown();
    return 0;
}
```

## Launch File Template (Python)

```python
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # Arguments
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    map_file = LaunchConfiguration('map')

    declare_use_sim = DeclareLaunchArgument(
        'use_sim_time', default_value='false',
        description='Use simulation clock')

    # Nodes
    my_node = Node(
        package='my_robot_pkg',
        executable='my_robot_node',
        name='my_robot',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'max_speed': 0.5,
            'update_rate': 20.0,
        }],
        remappings=[
            ('cmd_vel', '/robot/cmd_vel'),
            ('scan', '/robot/scan'),
        ]
    )

    # Include another launch file
    nav2_launch = IncludeLaunchDescription(
        PathJoinSubstitution([FindPackageShare('nav2_bringup'), 'launch', 'navigation_launch.py']),
        launch_arguments={'use_sim_time': use_sim_time}.items()
    )

    return LaunchDescription([
        declare_use_sim,
        my_node,
        nav2_launch,
    ])
```

## Nav2 Python API

```python
from nav2_simple_commander.robot_navigator import BasicNavigator, TaskResult
from geometry_msgs.msg import PoseStamped
import rclpy

rclpy.init()
navigator = BasicNavigator()

# Set initial pose
initial_pose = PoseStamped()
initial_pose.header.frame_id = 'map'
initial_pose.pose.position.x = 0.0
initial_pose.pose.position.y = 0.0
initial_pose.pose.orientation.w = 1.0
navigator.setInitialPose(initial_pose)

# Wait for Nav2 to be ready
navigator.waitUntilNav2Active()

# Navigate to pose
goal_pose = PoseStamped()
goal_pose.header.frame_id = 'map'
goal_pose.pose.position.x = 3.0
goal_pose.pose.position.y = 1.5
goal_pose.pose.orientation.w = 1.0

navigator.goToPose(goal_pose)

while not navigator.isTaskComplete():
    feedback = navigator.getFeedback()
    if feedback:
        print(f'Distance remaining: {feedback.distance_remaining:.2f}m')

result = navigator.getResult()
if result == TaskResult.SUCCEEDED:
    print('Goal reached!')
elif result == TaskResult.CANCELED:
    print('Canceled')
elif result == TaskResult.FAILED:
    print('Failed!')

# Follow waypoints
waypoints = [goal_pose_1, goal_pose_2, goal_pose_3]
navigator.followWaypoints(waypoints)
```

## Troubleshooting

```bash
# Check if nodes are talking
ros2 topic hz /topic          # is topic being published?
ros2 topic echo /topic        # what's in the messages?
ros2 node info /node_name     # what does node publish/subscribe?

# TF issues
ros2 run tf2_tools view_frames         # visualize TF tree
ros2 run tf2_ros tf2_echo map odom     # check specific transform

# QoS mismatch (silent failure!)
ros2 topic info /topic --verbose       # shows QoS of publishers/subscribers
# Mismatch: publisher is RELIABLE, subscriber is BEST_EFFORT → no data!
# Fix: match QoS profiles

# Node not found
ros2 pkg executables my_pkg           # list executables in package
colcon build --packages-select my_pkg # rebuild
source install/setup.bash             # re-source

# Log level
ros2 run my_pkg my_node --ros-args --log-level debug

# Environment
printenv | grep ROS                   # check ROS env vars
echo $ROS_DOMAIN_ID                   # domain ID (must match)
echo $RMW_IMPLEMENTATION              # DDS middleware in use
```
