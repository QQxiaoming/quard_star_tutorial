(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-5                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two external variables with the same name where both have a type declaration. :)
(:*******************************************************:)
declare variable $input-context as item()* external;
declare variable $input-context as item()*external;
1