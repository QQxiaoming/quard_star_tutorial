(: Name: ForExprType063 :)
(: Description: FLWOR expressions with type declaration. Attempt type declaration of node() on a set of items :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as node() in data( ($input-context//fs:Folder)[1]/fs:File/fs:FileName )
return $test