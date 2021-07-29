(:*******************************************************:)
(:Test: numberdec1args-2                                  :)
(:Written By: Carmelo Montanez                            :)
(:Date: Thu Dec 16 10:48:17 GMT-05:00 2004                :)
(:Purpose: Evaluates The "number" function               :)
(: with the arguments set as follows:                    :)
(:$arg = xs:decimal(mid range)                           :)
(:*******************************************************:)

fn:number(xs:decimal("617375191608514839")) eq 617375191608514839