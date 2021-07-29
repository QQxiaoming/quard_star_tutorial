(:*******************************************************:)
(: Test: K-ForExprWithout-25                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(for $xs:name in (1, 1) return $xs:name) instance of xs:integer+`. :)
(:*******************************************************:)
(for $xs:name in (1, 1) return $xs:name) instance of xs:integer+