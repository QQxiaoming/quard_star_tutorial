(: Name: ForExprType057 :)
(: Description: FLWOR expressions with type declaration. Type declaration matcing text nodes :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as text() in ($input-context//fs:Folder)[1]/fs:File/fs:FileName/text()
return $test