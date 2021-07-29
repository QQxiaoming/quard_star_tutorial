(:*******************************************************:)
(: Test: K-InternalVariablesWith-15                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: One prolog variable depending on itself.     :)
(:*******************************************************:)
declare variable $var1 := $var1;
true()