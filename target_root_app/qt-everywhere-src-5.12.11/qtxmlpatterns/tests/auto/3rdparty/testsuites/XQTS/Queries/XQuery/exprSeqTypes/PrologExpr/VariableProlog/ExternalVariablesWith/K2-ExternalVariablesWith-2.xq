(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-2                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two external variables with the same name where the last has a type declaration. :)
(:*******************************************************:)
declare variable $input-context external;
declare variable $input-context as item()* external;
1