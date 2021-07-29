(: Name: ForExprType008 :)
(: Description: Wrong order for positional and typing parts of FLWOR expression :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $file at $pos as element(Folder,xs:untyped) in ($input-context//Folder)[1]/File
return (string($file/FileName[1]), $pos)