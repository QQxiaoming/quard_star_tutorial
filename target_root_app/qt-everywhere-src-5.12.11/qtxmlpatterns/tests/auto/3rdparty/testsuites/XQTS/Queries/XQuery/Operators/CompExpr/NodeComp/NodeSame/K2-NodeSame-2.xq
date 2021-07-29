(:*******************************************************:)
(: Test: K2-NodeSame-2                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity is is correct between variable references. :)
(:*******************************************************:)
declare variable $var := <elem/>;
    $var is $var