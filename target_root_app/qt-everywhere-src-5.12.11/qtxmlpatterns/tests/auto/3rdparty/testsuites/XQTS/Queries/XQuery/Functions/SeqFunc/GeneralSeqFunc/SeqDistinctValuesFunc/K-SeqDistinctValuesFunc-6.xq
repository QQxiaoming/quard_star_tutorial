(:*******************************************************:)
(: Test: K-SeqDistinctValuesFunc-6                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `deep-equal(distinct-values( ("1", 1, 2, 1, 1, 3, 1, 1, 3, xs:anyURI("example.com/"), xs:anyURI("example.com/"))), ("1", 1, 2, 3, xs:anyURI("example.com/")))`. :)
(:*******************************************************:)
declare function local:canon($arg)
{
    for $i in for $s
              in $arg
              return string($s)
    order by $i
    return $i
};

deep-equal(local:canon(distinct-values(("1", 1, 2, 1, 1, 3, 1, 1, 3, xs:anyURI("example.com/"), xs:anyURI("example.com/")))),
           local:canon(("1", 1, 2, 3, xs:anyURI("example.com/"))))
