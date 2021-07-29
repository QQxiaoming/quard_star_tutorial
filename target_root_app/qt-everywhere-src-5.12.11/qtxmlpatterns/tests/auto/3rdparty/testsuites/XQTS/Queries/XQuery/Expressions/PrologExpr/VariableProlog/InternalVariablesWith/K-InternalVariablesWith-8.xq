(:*******************************************************:)
(: Test: K-InternalVariablesWith-8                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: '=' cannot be used to assign values in 'declare variable', it must be ':='. :)
(:*******************************************************:)
declare variable $var1 = 1; 1