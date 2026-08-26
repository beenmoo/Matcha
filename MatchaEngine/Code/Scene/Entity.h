namespace Matcha
{
class Entity
{
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual void Update();
    void SetActive(bool active) { mActive = active; }
    [[nodiscard]] bool IsActive() const { return mActive; }

private:
    bool mActive = true;
};
}