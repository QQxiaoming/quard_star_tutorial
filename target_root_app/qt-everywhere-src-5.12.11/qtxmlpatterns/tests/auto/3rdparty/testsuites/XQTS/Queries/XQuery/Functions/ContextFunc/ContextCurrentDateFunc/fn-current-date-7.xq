(:*******************************************************:)
(:Test: fn-current-date-7                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "fn-current-date" function used :)
(:as part of a "-" expression and a dayTimeDuration.     :)
(:*******************************************************:)

fn:current-date() - xs:dayTimeDuration("P3DT1H15M")
