(: Name: ForExprType019 :)
(: Description: Match a text node type :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName as text() in ($input-context//Folder)[1]/File/FileName/text()
return string( $fileName )
