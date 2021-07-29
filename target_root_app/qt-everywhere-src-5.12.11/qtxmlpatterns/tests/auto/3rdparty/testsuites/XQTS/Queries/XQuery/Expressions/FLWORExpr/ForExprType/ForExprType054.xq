(: Name: ForExprType054 :)
(: Description: FLWOR expressions with type declaration. Type declaration matching item() values :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as item() in data( ($input-context//fs:Folder)[1]/fs:File/fs:FileName )
return $test