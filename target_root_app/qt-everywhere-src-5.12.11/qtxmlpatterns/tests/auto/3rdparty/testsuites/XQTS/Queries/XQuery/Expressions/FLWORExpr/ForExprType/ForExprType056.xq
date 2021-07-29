(: Name: ForExprType056 :)
(: Description: FLWOR expressions with type declaration. Type declaration matching a document node :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as document-node() in $input-context
return ($test//fs:Folder)[1]/fs:FolderName