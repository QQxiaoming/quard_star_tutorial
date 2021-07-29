(:*******************************************************:)
(:Test: numberflt1args-1                                  :)
(:Written By: Carmelo Montanez                            :)
(:Date: Thu Dec 16 10:48:17 GMT-05:00 2004                :)
(:Purpose: Evaluates The "number" function               :)
(: with the arguments set as follows:                    :)
(:$arg = xs:float(lower bound)                           :)
(:*******************************************************:)

fn:number(xs:float("-3.4028235E38")) eq -3.4028234663852885E38