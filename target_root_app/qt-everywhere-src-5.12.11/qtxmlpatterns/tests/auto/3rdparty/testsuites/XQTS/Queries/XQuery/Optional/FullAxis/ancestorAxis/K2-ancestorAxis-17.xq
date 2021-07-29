(:*******************************************************:)
(: Test: K2-ancestorAxis-17                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply axis ancestor to a single document node. :)
(:*******************************************************:)
document {()}/ancestor::node(), count(document {()}/ancestor::node()), 1