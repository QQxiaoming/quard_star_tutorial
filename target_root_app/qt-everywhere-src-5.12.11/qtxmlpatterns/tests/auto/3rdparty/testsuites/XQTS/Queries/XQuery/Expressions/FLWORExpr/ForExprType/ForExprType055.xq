(: Name: ForExprType055 :)
(: Description: FLWOR expressions with type declaration. Type declaration matching node() values :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as node() in ($input-context//fs:Folder)[1]/fs:File
return $test/fs:FileName