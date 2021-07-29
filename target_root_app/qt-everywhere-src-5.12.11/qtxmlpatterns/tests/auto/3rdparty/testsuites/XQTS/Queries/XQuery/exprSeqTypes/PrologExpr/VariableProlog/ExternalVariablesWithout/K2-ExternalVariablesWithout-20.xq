(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-20                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure XPDY0002 is raised for global variables if there's no focus defined. Reference the variable twice. :)
(:*******************************************************:)
declare variable $global := count(*);
    <e/>/($global, $global)