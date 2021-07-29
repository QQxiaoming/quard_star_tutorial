(: Name: ForExprType007 :)
(: Description: Wrong order for type declaration :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file in ($input-context//Folder)[1]/File as element(Folder,xs:untypedAny)
return $file/FileName