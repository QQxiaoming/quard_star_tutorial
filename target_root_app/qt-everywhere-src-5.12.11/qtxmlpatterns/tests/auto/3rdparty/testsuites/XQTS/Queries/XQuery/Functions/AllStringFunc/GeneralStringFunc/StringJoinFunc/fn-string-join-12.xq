(:*******************************************************:)
(:Test: fn-string-join-12                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri June 24, 2005                                :)
(:Purpose: Evaluates The "string-join" function          :)
(:using it as a argument of a fn:not - returns true      :)
(:*******************************************************:)

fn:not(fn:string-join(("A"),"B"))