(: Name: ForExprType058 :)
(: Description: FLWOR expressions with type declaration. Type declaration matcing XML comment nodes :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as comment() in ($input-context//fs:Folder)[1]/comment()
return $test