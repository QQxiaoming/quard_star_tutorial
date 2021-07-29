(: Name: ReturnExpr020 :)
(: Description: 'return' expression containing a typed value constructor function :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File
return xs:string( data( $file/FileName[1] ))