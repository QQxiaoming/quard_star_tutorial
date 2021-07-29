(: Name: ForExprType067 :)
(: Description: FLWOR expressions with type declaration. Attempt type declaration of comment() on a set of values :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as comment() in data( ($input-context//fs:Folder)[1]/fs:File/fs:FileName )
return $test