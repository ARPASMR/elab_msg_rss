unpack_nogui:
	cd xrit; unzip -o -: main_src.zip; cd ..


comp_nogui:
	make -C xrit OS=\"linux\" GTK_REL=\"no\"


unpack_gui:
	cd xrit; unzip -o -: main_src.zip; cd ..
	cd sgtk; unzip -o -: sgtk_src.zip; cd ..

comp_gui:
	make -C xrit OS=\"linux\" GTK_REL=\"2.0\"


comp_gui1:
	make -C xrit OS=\"linux\"


