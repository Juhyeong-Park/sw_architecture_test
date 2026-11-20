from setuptools import setup

package_name = 'demo_localization'

setup(
    name=package_name,
    version='0.1.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='imova-demo',
    maintainer_email='dev@example.com',
    description='Demo localization node',
    license='Apache-2.0',
    entry_points={
        'console_scripts': ['localization_node = demo_localization.localization_node:main'],
    },
)
