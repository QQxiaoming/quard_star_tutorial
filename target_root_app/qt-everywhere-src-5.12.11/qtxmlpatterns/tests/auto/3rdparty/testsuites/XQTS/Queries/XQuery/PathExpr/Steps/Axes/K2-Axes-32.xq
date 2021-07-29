(:*******************************************************:)
(: Test: K2-Axes-32                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply axis '..' to the return value of fn:root(). :)
(:*******************************************************:)
declare variable $myVar := <e/>;
            empty(fn:root($myVar/(/)/..))