(: FileName: CondExpr018 :)
(: Purpose: if where branches return different valid types :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return
	if( $file/FileName='File00000000004' )
	then 1
	else data( $file/FileName )