(: Name: ForExprType061 :)
(: Description: FLWOR expressions with type declaration. Type declaration matcing PI nodes - no matching PI found :)

declare namespace fs="http://www.example.com/filesystem";

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

for $test as processing-instruction("unknown") in ($input-context//fs:Folder)[1]//processing-instruction()
return $test