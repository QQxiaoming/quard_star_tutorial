(: Name: ForExpr002 :)
(: Description: For+Return - error, undefined variable in 'IN' Expr :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $folder in $folder/File 
return <file name="{$folder/File/FileName}"/>

