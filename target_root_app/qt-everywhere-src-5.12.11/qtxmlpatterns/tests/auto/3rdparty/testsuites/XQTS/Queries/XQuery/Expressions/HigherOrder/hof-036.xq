xquery version "1.1";
(: Higher Order Functions :)
(: Heterogeneous sequence of atomics and functions on rhs of "/" - not clear if this is allowed :)
(: Author - Michael Kay, Saxonica :)

(<a b="3"/>/(string(@b), upper-case#1, 17))[. instance of xs:anyAtomicType]

