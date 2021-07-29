(:*******************************************************:)
(: Test: K2-Axes-33                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply axis '..' to a '/' step.               :)
(:*******************************************************:)
declare variable $myVar := document { <e/>};
        empty($myVar/(/)/..)