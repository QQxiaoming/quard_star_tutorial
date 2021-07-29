(:*******************************************************:)
(: Test: K2-NodeTest-33                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Trigger XPTY0018 inside a body for a global variable. :)
(:*******************************************************:)
declare variable $myVariable := <e/>/(1, <e/>);
$myVariable