(: Name: ForExprType062 :)
(: Description: FLWOR expressions with type declaration. Attempt type declaration of item() on a set of nodes :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as item() in ($input-context//fs:Folder)[1]/fs:File/fs:FileName
return $test