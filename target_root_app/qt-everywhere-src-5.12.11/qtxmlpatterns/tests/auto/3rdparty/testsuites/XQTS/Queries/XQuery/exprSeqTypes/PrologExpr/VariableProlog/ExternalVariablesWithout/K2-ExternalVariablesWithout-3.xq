(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-3                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An external variable using an undeclared prefix. :)
(:*******************************************************:)
declare default element namespace "";
declare namespace prefix = "";
declare variable $prefix:input-context external;
1