from setuptools import setup

setup( name='eziradebugnode',
       version='0.1',
       description='A wrapper for launching and interacting with a Steem Debug Node',
       url='http://github.com/eziranetwork/ezira',
       author='Ezira Network.',
       author_email='vandeberg@ezira.io',
       license='See LICENSE.md',
       packages=['eziradebugnode'],
       #install_requires=['steemapi'],
       zip_safe=False )