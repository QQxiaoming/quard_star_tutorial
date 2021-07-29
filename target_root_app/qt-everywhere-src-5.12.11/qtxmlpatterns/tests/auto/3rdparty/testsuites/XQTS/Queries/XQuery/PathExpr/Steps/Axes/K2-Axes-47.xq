(:*******************************************************:)
(: Test: K2-Axes-47                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply '/' to a variable.                     :)
(:*******************************************************:)
declare variable $myVar := <e/>;
empty($myVar/descendant-or-self::text())