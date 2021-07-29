(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-18                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: XQuery 1.0 doesn't allow externals to have a default value. :)
(:*******************************************************:)
declare variable $var external := 1; 1