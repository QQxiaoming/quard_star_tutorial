(:*******************************************************:)
(: Test: K2-InternalVariablesWithout-11                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable initialized with a function that doesn't exist. :)
(:*******************************************************:)
declare variable $local:myVar := local:thisFunctionDoesNotExist();
1