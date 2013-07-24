# vi: set ft=ruby :

share_name = "vagrant_share"
share_guest_dir = "/home/vagrant/share"

Vagrant::Config.run do |config|
  config.vm.box = "ubuntu-12.04-64bit"
  config.vm.box_url = "http://felixge.s3.amazonaws.com/12/ubuntu-12.04-64bit.box"
  config.vm.share_folder(share_name, share_guest_dir, ".")
  config.vm.customize ["setextradata", :id, "VBoxInternal2/SharedFoldersEnableSymlinksCreate/#{share_name}", "1"]
  #config.vm.customize ["modifyvm", :id, "--memory", 2048, "--cpus", 8]
  # Runs every time on `vagrant up`, configures up the machine & does a build
  #config.vm.provision :shell, :inline => "bash #{share_guest_dir}/scripts/vagrant/provision.sh"
end
