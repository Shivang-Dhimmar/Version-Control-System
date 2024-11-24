all:
	cd src/file_operations && $(MAKE)
	cd src/snapshot_management && $(MAKE)

clean:
	cd src/snapshot_management && $(MAKE) clean
	cd src/file_operations && $(MAKE) clean
	