(:*******************************************************:)
(:Test: fn-current-time-7                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "fn-current-time" function used :)
(:as part of a "-" expression and a dayTimeDuration.     :)
(:*******************************************************:)

fn:current-time() - xs:dayTimeDuration("P3DT1H15M")
