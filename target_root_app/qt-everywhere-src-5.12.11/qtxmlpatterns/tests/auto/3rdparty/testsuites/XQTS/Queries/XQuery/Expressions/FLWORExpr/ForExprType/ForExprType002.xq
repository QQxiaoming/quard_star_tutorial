(: Name: ForExprType002 :)
(: Description: FLWOR expressions with type declaration (attribute type) :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName as attribute(name,xs:untypedAtomic) in ($input-context//Folder)[1]/File/@name
return data($fileName)