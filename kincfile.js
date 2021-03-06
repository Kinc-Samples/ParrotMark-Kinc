let project = new Project('ParrotMark');

project.addFile('Sources/**');
project.setDebugDir('Deployment');

resolve(project);
