(: FileName: CondExpr017 :)
(: Purpose: if where branches return different valid types :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return
	if( $file/FileName='File00000000000' )
	then $file/FileName
	else data( $file/FileName )