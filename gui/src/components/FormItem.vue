<template>
    <div class="form-item">
        <div class="form-item-label" v-if="label">
            {{ label }}
        </div>
        <div class="form-item-content"
            :class="{ alignRight: alignContent == 'right',
                      alignCenter: alignContent == 'center'}">
            <slot></slot>
        </div>
    </div>
</template>

<script>
export default {
    name: 'FormItem',
    props: {
        label: String,
        alignContent: String
    }
}
</script>

<style scoped>
.form-item {
    display: flex;
    flex-direction: column;
}
.form-item-label {
    font-size: 0.85rem;
    padding: 0.1rem 0.1rem 0.1rem 0.3rem;
}
.form-item-content {
    padding-top: 0.3rem;
    padding-bottom: 0.3rem;
    display: flex;
    flex-direction: row;
}
.alignRight {
    align-self: flex-end;
}
.alignCenter {
    align-self: center;
}
</style>