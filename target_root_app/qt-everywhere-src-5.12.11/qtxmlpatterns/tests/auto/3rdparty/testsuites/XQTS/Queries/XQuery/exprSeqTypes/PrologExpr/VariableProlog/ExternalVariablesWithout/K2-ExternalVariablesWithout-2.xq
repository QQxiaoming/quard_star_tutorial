(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-2                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two external variables with the same name and where the variable is used. :)
(:*******************************************************:)
declare variable $input-context external;
declare variable $input-context external;
$input-context