(:*******************************************************:)
(: Test: K2-ExternalVariablesWithout-22                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use the global focus in various ways three references. :)
(:*******************************************************:)
declare variable $global := count(*);
<wrongFocus>
    <e1/>
    <e2/>
</wrongFocus>/$global