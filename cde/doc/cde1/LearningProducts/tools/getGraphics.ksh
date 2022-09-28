#! /bin/ksh

for file in `cat /doc/Rivers/graphics/sourceNames`
do
  oldfile=/doc/Rivers/admin/graphics.old/xwd/$file

  if [[ -f $oldfile ]]
  then
  	print "Copying $file ..."
	cp $oldfile /doc/Rivers/graphics/Source
  else
	print "Making placeholder for $file ..."
	cp /doc/Rivers/graphics/Source/Missing.xwd \
	   /doc/Rivers/graphics/Source/$file
  fi
done
print "Done."
