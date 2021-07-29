(:*******************************************************:)
(: Test: K2-SeqExprTreat-3                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Lhs must be a StepExpr, and TreatExpr is not. :)
(:*******************************************************:)
fn:root(self::node()) treat as document-node()/X