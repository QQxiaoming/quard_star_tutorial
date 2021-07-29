(:*******************************************************:)
(: Test: K2-NodeSame-4                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure node identity of nodes used with node constructors. :)
(:*******************************************************:)
declare variable $e := attribute name {()};
    $e is $e,
    <is/> is <is/>