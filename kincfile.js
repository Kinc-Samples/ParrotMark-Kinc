let project = new Project('ParrotMark');

project.cpp = true;
project.addFile('Sources/**');
project.setDebugDir('Deployment');

resolve(project);
