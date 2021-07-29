(:*******************************************************:)
(: Test: K-InternalVariablesWith-9                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A prolog variable depending on a variable which is not in scope. :)
(:*******************************************************:)
declare variable $var1 := $var2; 
declare variable $var2 := 2;
$var1