import { defineConfig } from 'vitepress'

export default defineConfig({
  title: "AI Robotics Engineering",
  description: "The Ultimate Compendium for Electronics Engineering, Robotics & Physical AI",
  srcDir: '../', // Root directory is the source for markdown files
  outDir: './.vitepress/dist',
  base: '/ai-robotics-engineering/', // Required for GitHub Pages
  ignoreDeadLinks: true, // Don't fail the build if an external link is slow/dead
  themeConfig: {
    search: {
      provider: 'local'
    },
    nav: [
      { text: 'Home', link: '/' },
      { text: 'GitHub', link: 'https://github.com/aredwan-xyz/ai-robotics-engineering' }
    ],
    sidebar: [
      {
        text: 'Roadmaps',
        collapsed: false,
        items: [
          { text: 'Electronics Engineer', link: '/roadmaps/electronics-engineer' },
          { text: 'Embedded Systems', link: '/roadmaps/embedded-systems' },
          { text: 'Robotics Engineer', link: '/roadmaps/robotics-engineer' },
          { text: 'Drone Engineer', link: '/roadmaps/drone-engineer' },
          { text: 'Physical AI', link: '/roadmaps/physical-ai' }
        ]
      },
      {
        text: 'Projects',
        collapsed: false,
        items: [
          { text: 'Beginner: LED Controller', link: '/projects/beginner/01-smart-led-controller' },
          { text: 'Intermediate: Custom BMS', link: '/projects/intermediate/01-custom-bms-lipo' },
          { text: 'Intermediate: Self-Balancing Robot', link: '/projects/intermediate/02-self-balancing-robot' },
          { text: 'Intermediate: ROS2 SLAM Mapper', link: '/projects/intermediate/03-ros2-lidar-mapper' },
          { text: 'Advanced: Sim-to-Real', link: '/projects/advanced/01-sim-to-real-locomotion' }
        ]
      },
      {
        text: 'Guides & References',
        collapsed: false,
        items: [
          { text: 'Electronics Reference', link: '/cheatsheets/electronics-quick-reference' },
          { text: 'ROS 2 Cheatsheet', link: '/cheatsheets/ros2-cheatsheet' },
          { text: 'Hardware Debugging', link: '/guides/hardware-debugging' },
          { text: 'Contribution Guide', link: '/CONTRIBUTING' }
        ]
      }
    ],
    socialLinks: [
      { icon: 'github', link: 'https://github.com/aredwan-xyz/ai-robotics-engineering' }
    ],
    footer: {
      message: 'Released under the CC0 1.0 Universal License.',
      copyright: 'Created by Abid Redwan of CodeBeez.'
    }
  }
})
