(: Name: ForExprType001 :)
(: Description: FLWOR expressions with type declaration (element type) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file as element(File,xs:untyped) in ($input-context//Folder)[1]/File
return $file/FileName