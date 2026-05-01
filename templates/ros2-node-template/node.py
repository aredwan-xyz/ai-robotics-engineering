#!/usr/bin/env python3
"""
ROS 2 Node Template
Copy this, rename the class, update the topics/services, build.
"""
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy
from std_msgs.msg import String
from geometry_msgs.msg import Twist
from sensor_msgs.msg import LaserScan
from rcl_interfaces.msg import SetParametersResult
import numpy as np


class MyNode(Node):
    """
    Template ROS 2 node with:
    - Parameters with runtime updates
    - Publisher + Subscriber
    - Timer-based control loop
    - QoS profiles
    - Proper shutdown handling
    """

    def __init__(self):
        super().__init__('my_node')

        # --- Parameters ---
        self.declare_parameter('update_rate_hz', 10.0)
        self.declare_parameter('max_speed', 0.5)
        self.declare_parameter('debug', False)

        self.update_rate = self.get_parameter('update_rate_hz').value
        self.max_speed = self.get_parameter('max_speed').value
        self.debug = self.get_parameter('debug').value

        # Allow runtime parameter updates
        self.add_on_set_parameters_callback(self.parameters_callback)

        # --- QoS Profiles ---
        # Reliable + latched (for things that must be received, e.g., maps)
        reliable_latched = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
        )
        # Best-effort (for sensors where latest matters more than delivery)
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            durability=DurabilityPolicy.VOLATILE,
            history=HistoryPolicy.KEEP_LAST,
            depth=1,
        )

        # --- Publishers ---
        self.cmd_vel_pub = self.create_publisher(Twist, 'cmd_vel', 10)
        self.status_pub = self.create_publisher(String, 'status', reliable_latched)

        # --- Subscribers ---
        self.scan_sub = self.create_subscription(
            LaserScan,
            'scan',
            self.scan_callback,
            sensor_qos,
        )

        # --- Timer ---
        self.timer = self.create_timer(
            1.0 / self.update_rate,
            self.control_loop,
        )

        # --- State ---
        self.latest_scan: LaserScan | None = None

        self.get_logger().info(
            f'Node started — rate={self.update_rate}Hz, max_speed={self.max_speed}'
        )

    # --- Callbacks ---

    def scan_callback(self, msg: LaserScan):
        self.latest_scan = msg
        if self.debug:
            self.get_logger().debug(f'Scan received: {len(msg.ranges)} points')

    def control_loop(self):
        if self.latest_scan is None:
            return

        ranges = np.array(self.latest_scan.ranges, dtype=np.float32)
        # Replace inf/nan with max range
        ranges = np.where(np.isfinite(ranges), ranges, self.latest_scan.range_max)

        # Front obstacle distance
        n = len(ranges)
        front = ranges[n // 2 - 15 : n // 2 + 15]
        min_front = float(np.min(front))

        cmd = Twist()
        if min_front > 0.6:
            cmd.linear.x = min(self.max_speed, min_front * 0.3)
        else:
            cmd.angular.z = 0.5

        self.cmd_vel_pub.publish(cmd)

    def parameters_callback(self, params) -> SetParametersResult:
        for p in params:
            if p.name == 'max_speed':
                self.max_speed = p.value
                self.get_logger().info(f'max_speed updated to {self.max_speed}')
            elif p.name == 'debug':
                self.debug = p.value
        return SetParametersResult(successful=True)

    def destroy_node(self):
        # Stop robot before shutting down
        self.cmd_vel_pub.publish(Twist())
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = MyNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
