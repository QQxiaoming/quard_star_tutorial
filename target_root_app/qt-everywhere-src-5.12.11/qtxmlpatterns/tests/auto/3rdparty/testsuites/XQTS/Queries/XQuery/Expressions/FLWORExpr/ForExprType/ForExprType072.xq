(: Name: ForExprType072 :)
(: Description: FLWOR expressions with type declaration. Attempt type declaration of document-node() on a set of element nodes :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as document-node() in ($input-context//fs:Folder)[1]/fs:File/fs:FileName
return $test