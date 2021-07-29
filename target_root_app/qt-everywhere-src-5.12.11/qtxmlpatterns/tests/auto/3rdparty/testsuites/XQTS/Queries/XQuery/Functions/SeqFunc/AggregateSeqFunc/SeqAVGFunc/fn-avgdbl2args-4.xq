(:*******************************************************:)
(:Test: avgdbl2args-4                                     :)
(:Written By: Carmelo Montanez                            :)
(:Date: Fri Dec 10 10:15:47 GMT-05:00 2004                :)
(:Purpose: Evaluates The "avg" function                  :)
(: with the arguments set as follows:                    :)
(:$arg1 = xs:double(lower bound)                         :)
(:$arg2 = xs:double(mid range)                           :)
(:*******************************************************:)

fn:avg((xs:double("-1.7976931348623157E308"),xs:double("0"))) eq -8.9884656743115785E307