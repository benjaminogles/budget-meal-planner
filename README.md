
# Budget Meal Planner

![Screenshot](/groceries-tab.png)

This is a simple, offline desktop app for storing recipes, creating meal plans and generating grocery lists with your budget in mind.
Grocery lists are generated automatically from the meal plan.
It displays the price of recipes and the grocery list according to price data you enter for each type of food.

# Dependencies

- C++14
- Qt 5.12
- SQLite

# Build

```
git clone $repo_url $repo_dir
cd $repo_dir
qmake
make
```

# Run
```
# run without --db option to use temporary in-memory database
./budget-meal-planner --db file-name-for-new-database.db
```

# Concepts

Recipes reference zero or more ingredients.
Ingredients reference one food and have an associated quantity.
Foods can be staples or non-staples and have an associated price.
Groceries reference one food and an associated quantity, often derived from ingredient quantities.
The ingredient quantity is interpreted differently for staples and non-staples.

A food is considered a staple if you buy it in bulk and use it for many meals.
For staples, the ingredient quantity field is ignored when generating grocery lists and estimating prices.
Instead, the quantity is implicitly fixed to `1`.
This ensures that the list and estimated price is correct even if multiple recipes in a plan require that food.
It is assumed that no sane combination of recipes would require you to purchase more of any staple then you usually do in one shopping trip.
You should set the price of staples to how much it costs you to buy the food once.

For non-staple foods, you should set the price to how much it costs to buy one "unit" of the food.
The ingredient quantities will be summed and multiplied by this price.

## Examples

A bag of rice would be a staple food.

A green bell pepper would be a non-staple food.
In recipes that use green bell pepper, you would want to avoid using specific units for the bell pepper ingredient.
For example, using '1/2 cup bell pepper' (meaning diced) would throw off the price estimate unless `1/2 cup == 1/2 pepper`.
Instead, leave the units for non-staple foods unspecified and set the quantity for the number of whole units you will buy for that recipe.

Oranges may be used as a staple (bag of oranges that you eat throughout the week) or a non-staple (1 orange used in a recipe).
You can create two separate foods for this purpose: 'Oranges' and 'Orange'.
If you buy oranges on the same week that you plan the orange-using recipe, you will just have to remember to remove the lone orange from your grocery list.
There is probably a simple way to support this use case better but it is not implemented.

# Use

Step by step instructions for each task.

## Meal Plan

1. Go to Groceries tab
2. Enter recipe names under Planned Recipes (names will auto complete)

The grocery list will be updated along with the plan.
The list can be regenerated with the corresponding button (useful after editing food or recipe data).
You can clear the plan with the corresponding button.

## Add Other Groceries

1. Go to Groceries tab
2. Enter food name under Generated List (names will auto complete)

Groceries added manually will be maintained separately from auto-generated groceries so they will not be removed erroneously when changing the meal plan.

## Adding Recipe

1. Go to Recipes tab
2. Click Add
3. Change title
4. Enter steps
5. Enter ingredient names
6. Change ingredient units and quantities
7. Click Done

## Editing Recipe

1. Go to Recipes tab
2. Double click on recipe
3. Click Done when finished

You can also edit a recipe from the Groceries tab by double clicking a planned recipe.

## Adding Food

Foods are created lazily if necessary when ingredients or groceries are added with an unrecognized name.
Otherwise:

1. Go to Foods tab
2. Enter name text input
3. Hit return
4. If food is a staple, change staple field to 1
6. Enter price of purchasing this food once

## Edit Table Field

1. Double click on field or start typing while field is focused
2. Field will auto complete with valid options if applicable

## Remove Rows From Table

1. Select rows you want to remove (click each)
2. Click associated Delete button
3. Confirm

