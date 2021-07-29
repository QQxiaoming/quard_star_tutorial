(: Name: ForExprType059 :)
(: Description: FLWOR expressions with type declaration. Type declaration matcing PI nodes :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as processing-instruction() in ($input-context//fs:Folder)[1]//processing-instruction()
return $test