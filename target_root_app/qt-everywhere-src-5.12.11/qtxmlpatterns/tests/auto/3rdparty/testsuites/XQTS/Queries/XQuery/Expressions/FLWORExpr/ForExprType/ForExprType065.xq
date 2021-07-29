(: Name: ForExprType065 :)
(: Description: FLWOR expressions with type declaration. Attempt type declaration of text() on a set of values :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as text() in data( ($input-context//fs:Folder)[1]/fs:File/fs:FileName )
return $test