(: Name: ForExprType020 :)
(: Description: Match a 'item' type :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $fileName as item() in data( ($input-context//Folder)[1]/File/FileName )
return $fileName

